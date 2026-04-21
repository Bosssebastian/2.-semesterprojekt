<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLHeadingElement>, HTMLHeadingElement>;

	let {
		class: className,
		ref = $bindable(null),
		children,
		...restProps
	}: Props = $props();
</script>

<h3
	bind:this={ref}
	data-slot="card-title"
	class={cn("text-lg font-semibold tracking-tight", className)}
	{...restProps}
>
	{@render children?.()}
</h3>
